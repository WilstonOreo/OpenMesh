/*===========================================================================*\
 *                                                                           *
 *                               OpenMesh                                    *
 *      Copyright (C) 2001-2011 by Computer Graphics Group, RWTH Aachen      *
 *                           www.openmesh.org                                *
 *                                                                           *
 *---------------------------------------------------------------------------* 
 *  This file is part of OpenMesh.                                           *
 *                                                                           *
 *  OpenMesh is free software: you can redistribute it and/or modify         * 
 *  it under the terms of the GNU Lesser General Public License as           *
 *  published by the Free Software Foundation, either version 3 of           *
 *  the License, or (at your option) any later version with the              *
 *  following exceptions:                                                    *
 *                                                                           *
 *  If other files instantiate templates or use macros                       *
 *  or inline functions from this file, or you compile this file and         *
 *  link it with other files to produce an executable, this file does        *
 *  not by itself cause the resulting executable to be covered by the        *
 *  GNU Lesser General Public License. This exception does not however       *
 *  invalidate any other reasons why the executable file might be            *
 *  covered by the GNU Lesser General Public License.                        *
 *                                                                           *
 *  OpenMesh is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU Lesser General Public License for more details.                      *
 *                                                                           *
 *  You should have received a copy of the GNU LesserGeneral Public          *
 *  License along with OpenMesh.  If not,                                    *
 *  see <http://www.gnu.org/licenses/>.                                      *
 *                                                                           *
\*===========================================================================*/ 

/*===========================================================================*\
 *                                                                           *             
 *   $Revision: 362 $                                                         *
 *   $Date: 2011-01-26 10:21:12 +0100 (Mi, 26 Jan 2011) $                   *
 *                                                                           *
\*===========================================================================*/

#define OPENMESHAPPS_MESHVIEWERWIDGET_CC

//== INCLUDES =================================================================

#ifdef _MSC_VER
//#  pragma warning(disable: 4267 4311)
#endif

//
#include <iostream>
#include <fstream>
#include <QImage>
#include <QKeyEvent>
#include <OpenMesh/Core/Utils/vector_cast.hh>
#include <OpenMesh/Tools/Utils/Timer.hh>
#include <OpenMesh/Apps/VDProgMesh/Streaming/Client/MeshViewerWidgetT.hh>

using namespace OpenMesh;


//== IMPLEMENTATION ========================================================== 


template <typename M>
bool 
MeshViewerWidgetT<M>::open_mesh(const char* _filename,
                                IO::Options _opt)
{
  // load mesh
  // calculate normals
  // set scene center and radius   
  
  mesh_.request_face_normals();
  mesh_.request_face_colors();
  mesh_.request_vertex_normals();
  mesh_.request_vertex_texcoords2D();
  
  if ( IO::read_mesh(mesh_, _filename, _opt ))
  {
    opt_ = _opt;
    
    // update face and vertex normals     
    if ( ! opt_.check( IO::Options::FaceNormal ) )
      mesh_.update_face_normals();
    
    if ( ! opt_.check( IO::Options::VertexNormal ) )
      mesh_.update_vertex_normals();
    
    if ( mesh_.has_vertex_colors() )
      add_draw_mode("Colored");

    if ( _opt.check( IO::Options::FaceColor ) )
      add_draw_mode("Colored Faces");
    else
      mesh_.release_face_colors();

    // bounding box
    typename Mesh::ConstVertexIter vIt(mesh_.vertices_begin());
    typename Mesh::ConstVertexIter vEnd(mesh_.vertices_end());      
    
    typedef typename Mesh::Point Point;
    using OpenMesh::Vec3f;
    
    Vec3f bbMin, bbMax;
    
    bbMin = bbMax = OpenMesh::vector_cast<Vec3f>(mesh_.point(vIt));
    
    for (size_t count=0; vIt!=vEnd; ++vIt, ++count)
    {
      bbMin.minimize( OpenMesh::vector_cast<Vec3f>(mesh_.point(vIt)));
      bbMax.maximize( OpenMesh::vector_cast<Vec3f>(mesh_.point(vIt)));
      
      if ( ! opt_.check( IO::Options::VertexColor ) &&
	   mesh_.has_vertex_colors() )
      {
	typename Mesh::Color 
	  c( 54,
	     (unsigned char)(54.5+200.0*count/mesh_.n_vertices()),
	     54 );
	mesh_.set_color( vIt, c );
      }
    }
    
    
    // set center and radius
    set_scene_pos( (bbMin+bbMax)*0.5, (bbMin-bbMax).norm()*0.5 );
    
    // info
    std::clog << mesh_.n_vertices() << " vertices, "
	      << mesh_.n_edges()    << " edge, "
	      << mesh_.n_faces()    << " faces\n";
    
    //      
    {
      std::clog << "Computing strips.." << std::flush;
      OpenMesh::Utils::Timer t;
      t.start();
      compute_strips();
      t.stop();
      std::clog << "done [" << strips_.n_strips() 
		<< " strips created in " << t.as_string() << "]\n";
    }
    
#if defined(OM_CC_MSVC)
    updateGL();
#endif

    return true;
  }
  return false;
}


//-----------------------------------------------------------------------------

template <typename M>
bool MeshViewerWidgetT<M>::open_texture( const char *_filename )
{
   QImage texsrc;
   QString fname = _filename;

   if (texsrc.load( fname ))
   {      
     return set_texture( texsrc );
   }
   return false;
}


//-----------------------------------------------------------------------------

template <typename M>
bool MeshViewerWidgetT<M>::set_texture( QImage& _texsrc )
{
  std::clog << "set_texture\n";

  if ( !opt_.vertex_has_texcoord() )
    return false;
   
  {
    // adjust texture size: 2^k * 2^l
    int tex_w, w( _texsrc.width()  );
    int tex_h, h( _texsrc.height() );

    for (tex_w=1; tex_w <= w; tex_w <<= 1) ;
    for (tex_h=1; tex_h <= h; tex_h <<= 1) ;
    tex_w >>= 1;
    tex_h >>= 1;
    _texsrc = _texsrc.scaled( tex_w, tex_h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
  }

  QImage texture( QGLWidget::convertToGLFormat ( _texsrc ) );
  
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_SKIP_ROWS,   0);
  glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
  glPixelStorei(GL_UNPACK_ALIGNMENT,   1);
  glPixelStorei(GL_PACK_ROW_LENGTH,    0);
  glPixelStorei(GL_PACK_SKIP_ROWS,     0);
  glPixelStorei(GL_PACK_SKIP_PIXELS,   0);
  glPixelStorei(GL_PACK_ALIGNMENT,     1);    
  
  if ( tex_id_ > 0 )
  {
    glDeleteTextures(1, &tex_id_);
  }
  glGenTextures(1, &tex_id_);
  glBindTexture(GL_TEXTURE_2D, tex_id_);
    
  // glTexGenfv( GL_S, GL_SPHERE_MAP, 0 );
  // glTexGenfv( GL_T, GL_SPHERE_MAP, 0 );
    
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);      
  
  glTexImage2D(GL_TEXTURE_2D,       // target
	       0,                   // level
	       GL_RGBA,             // internal format
	       texture.width(),     // width  (2^n)
	       texture.height(),    // height (2^m)
	       0,                   // border
	       GL_RGBA,             // format
	       GL_UNSIGNED_BYTE,    // type
	       texture.bits() );    // pointer to pixels
  
  return true;
}


//-----------------------------------------------------------------------------

template <typename M>
void
MeshViewerWidgetT<M>::draw_openmesh(const std::string& _draw_mode)
{
  typename Mesh::ConstFaceIter    fIt(mesh_.faces_begin()), 
                                  fEnd(mesh_.faces_end());

  typename Mesh::ConstFaceVertexIter fvIt;

#if defined(OM_USE_OSG) && OM_USE_OSG
  if (_draw_mode == "OpenSG Indices") // --------------------------------------
  {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, mesh_.points());

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());

    if ( tex_id_ && mesh_.has_vertex_texcoords2D() )
    {
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2, GL_FLOAT, 0, mesh_.texcoords2D());
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, tex_id_);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, tex_mode_);
    }

    glDrawElements(GL_TRIANGLES, 
		   mesh_.osg_indices()->size(), 
		   GL_UNSIGNED_INT, 
		   &mesh_.osg_indices()->getField()[0] );

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  }
  else
#endif

  if (_draw_mode == "Wireframe") // --------------------------------------------
  {
     glBegin(GL_TRIANGLES);
     for (; fIt!=fEnd; ++fIt)
     {
        fvIt = mesh_.cfv_iter(fIt.handle()); 
        glVertex3fv( &mesh_.point(fvIt)[0] );
        ++fvIt;
        glVertex3fv( &mesh_.point(fvIt)[0] );
        ++fvIt;
        glVertex3fv( &mesh_.point(fvIt)[0] );
     }
     glEnd();
  }
  
  else if (_draw_mode == "Solid Flat") // -------------------------------------
  {
    glBegin(GL_TRIANGLES);
    for (; fIt!=fEnd; ++fIt)
    {
      glNormal3fv( &mesh_.normal(fIt)[0] );
      
      fvIt = mesh_.cfv_iter(fIt.handle()); 
      glVertex3fv( &mesh_.point(fvIt)[0] );
      ++fvIt;
      glVertex3fv( &mesh_.point(fvIt)[0] );
      ++fvIt;
      glVertex3fv( &mesh_.point(fvIt)[0] );
    }
    glEnd();


  }


  else if (_draw_mode == "Solid Smooth") // -----------------------------------
  {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, mesh_.points());

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());

    if ( tex_id_ && mesh_.has_vertex_texcoords2D() )
    {
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2, GL_FLOAT, 0, mesh_.texcoords2D());
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, tex_id_);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, tex_mode_);
    }

    glBegin(GL_TRIANGLES);
    for (; fIt!=fEnd; ++fIt)
    {
      fvIt = mesh_.cfv_iter(fIt.handle()); 
      glArrayElement(fvIt.handle().idx());
      ++fvIt;
      glArrayElement(fvIt.handle().idx());
      ++fvIt;
      glArrayElement(fvIt.handle().idx());
    }
    glEnd();
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    if ( tex_id_ && mesh_.has_vertex_texcoords2D() )
    {
      glDisable(GL_TEXTURE_2D);
    }
  }  

  else if (_draw_mode == "Colored") // ----------------------------------------
  {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, mesh_.points());

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());

    if ( mesh_.has_vertex_colors() )
    {
      glEnableClientState( GL_COLOR_ARRAY );
      glColorPointer(3, GL_UNSIGNED_BYTE, 0,mesh_.vertex_colors());
    }

    glBegin(GL_TRIANGLES);
    for (; fIt!=fEnd; ++fIt)
    {
      fvIt = mesh_.cfv_iter(fIt.handle()); 
      glArrayElement(fvIt.handle().idx());
      ++fvIt;
      glArrayElement(fvIt.handle().idx());
      ++fvIt;
      glArrayElement(fvIt.handle().idx());
    }
    glEnd();
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
  }  


  else if (_draw_mode == "Colored Faces") // ----------------------------------
  {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, mesh_.points());

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());

    glBegin(GL_TRIANGLES);
    for (; fIt!=fEnd; ++fIt)
    {
      glColor( fIt.handle() );

      fvIt = mesh_.cfv_iter(fIt.handle()); 
      glArrayElement(fvIt.handle().idx());
      ++fvIt;
      glArrayElement(fvIt.handle().idx());
      ++fvIt;
      glArrayElement(fvIt.handle().idx());
    }
    glEnd();
    
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
  }  


  else if ( _draw_mode == "Strips'n VertexArrays" ) // -------------------------
  {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, mesh_.points());

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, mesh_.vertex_normals());

    if ( tex_id_ && mesh_.has_vertex_texcoords2D() )
    {
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2, GL_FLOAT, 0, mesh_.texcoords2D());
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, tex_id_);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, tex_mode_);
    }

    typename MyStripifier::StripsIterator strip_it = strips_.begin();
    typename MyStripifier::StripsIterator strip_last = strips_.end();

    // Draw all strips
    for (; strip_it!=strip_last; ++strip_it)
    {
      glDrawElements(GL_TRIANGLE_STRIP, 
		     strip_it->size(), GL_UNSIGNED_INT, &(*strip_it)[0] );
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  }


  else if (_draw_mode == "Show Strips" && strips_.is_valid() ) // -------------
  {
    typename MyStripifier::StripsIterator strip_it = strips_.begin();
    typename MyStripifier::StripsIterator strip_last = strips_.end();

    float cmax  = 256.0f;
    int   range = 220;
    int   base  = (int)cmax-range;
    int   drcol  = 13;
    int   dgcol  = 31;
    int   dbcol  = 17;
    
    int rcol=0, gcol=dgcol, bcol=dbcol+dbcol;
         
    // Draw all strips
    for (; strip_it!=strip_last; ++strip_it)
    {
      typename MyStripifier::IndexIterator idx_it   = strip_it->begin();
      typename MyStripifier::IndexIterator idx_last = strip_it->end();
      
      rcol = (rcol+drcol) % range;
      gcol = (gcol+dgcol) % range;
      bcol = (bcol+dbcol) % range;
      
      glBegin(GL_TRIANGLE_STRIP);
      glColor3f((rcol+base)/cmax, (gcol+base)/cmax, (bcol+base)/cmax);
      for ( ;idx_it != idx_last; ++idx_it )
        glVertex3fv( &mesh_.point( OM_TYPENAME Mesh::VertexHandle(*idx_it))[0] );
      glEnd();
    }
    glColor3f(1.0, 1.0, 1.0);
  }


  else if( _draw_mode == "Points" ) // -----------------------------------------
  {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, mesh_.points());
    glDrawArrays( GL_POINTS, 0, mesh_.n_vertices() );
    glDisableClientState(GL_VERTEX_ARRAY);
  }


}


//-----------------------------------------------------------------------------


template <typename M>
void 
MeshViewerWidgetT<M>::draw_scene(const std::string& _draw_mode)
{
  
  if ( ! mesh_.n_vertices() )
    return;
   
#if defined(OM_USE_OSG) && OM_USE_OSG
  else if ( _draw_mode == "OpenSG Indices")
  {     
    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    draw_openmesh( _draw_mode );
  }     
  else
#endif
  if ( _draw_mode == "Points" )
  {
    glDisable(GL_LIGHTING);
    draw_openmesh(_draw_mode);
  }
  else if (_draw_mode == "Wireframe")
  {
    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    draw_openmesh(_draw_mode);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  
  else if ( _draw_mode == "Hidden-Line" )
  {
    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDepthRange(0.01, 1.0);
    draw_openmesh("Solid Smooth");
    
    glDisable(GL_LIGHTING);
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
    glColor4f( 0.4f, 0.4f, 0.4f, 1.0f );
    glDepthRange( 0.0, 1.0 );
    draw_openmesh( "Wireframe" );
    
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);          
  }
  
  else if (_draw_mode == "Solid Flat")
  {
    glEnable(GL_LIGHTING);
    glShadeModel(GL_FLAT);
    draw_openmesh(_draw_mode);
  }
  
  else if (_draw_mode == "Solid Smooth"        ||
	   _draw_mode == "Strips'n VertexArrays" )
  {
    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    draw_openmesh(_draw_mode);
  }
  
  else if (_draw_mode == "Show Strips")
  {
    glDisable(GL_LIGHTING);
    draw_openmesh(_draw_mode);
  }
  
  else if (_draw_mode == "Colored" )
  {
    glDisable(GL_LIGHTING);
    draw_openmesh(_draw_mode);
  }

  else if (_draw_mode == "Colored Faces" )
  {
    glDisable(GL_LIGHTING);
    draw_openmesh(_draw_mode);
  }
}


//-----------------------------------------------------------------------------

#define TEXMODE( Mode ) \
   tex_mode_ = Mode; std::cout << "Texture mode set to " << #Mode << std::endl

template <typename M>
void 
MeshViewerWidgetT<M>::keyPressEvent( QKeyEvent* _event)
{
  switch( _event->key() )
  {
    case Qt::Key_I:
      std::cout << "\n# Vertices     : " << mesh_.n_vertices() << std::endl;
      std::cout << "# Edges        : " << mesh_.n_edges()    << std::endl;
      std::cout << "# Faces        : " << mesh_.n_faces()    << std::endl;
      std::cout << "binary  input  : " << opt_.check(opt_.Binary) << std::endl;
      std::cout << "swapped input  : " << opt_.check(opt_.Swap) << std::endl;
      std::cout << "vertex normal  : " 
		<< opt_.check(opt_.VertexNormal) << std::endl;
      std::cout << "vertex texcoord: " 
		<< opt_.check(opt_.VertexTexCoord) << std::endl;
      std::cout << "vertex color   : " 
		<< opt_.check(opt_.VertexColor) << std::endl;
      this->QGLViewerWidget::keyPressEvent( _event );
      break;

    case Qt::Key_T:
      switch( tex_mode_ )
      {
	case GL_MODULATE: TEXMODE(GL_DECAL); break;
	case GL_DECAL:    TEXMODE(GL_BLEND); break;
	case GL_BLEND:    TEXMODE(GL_REPLACE); break;
	case GL_REPLACE:  TEXMODE(GL_MODULATE); break;
      }
      updateGL();
      break;

    default:
      this->QGLViewerWidget::keyPressEvent( _event );
  }
}

#undef TEXMODE

//=============================================================================
