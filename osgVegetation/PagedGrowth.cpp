/****************************************************************************
*                                                                           *
* HiFiEngine                                                                *
* Copyright (C)2003 - 2005 Johan Hedstrom                                   *
* Email: hifiengine@gmail.com                                               *
* Web page: http://n00b.dyndns.org/HiFiEngine                               *
*                                                                           *
* HiFiEngine is only used with knowledge from the author. This software     *
* is not allowed to redistribute without permission from the author.        *
* For further license information, please turn to the product home page or  *
* contact author. Abuse against the HiFiEngine license is prohibited by law.*
*                                                                           *
*****************************************************************************/ 

#include "PagedGrowth.h"
#include <algorithm>
#include "tinyxml.h"

namespace osgVegetation
{

	PagedGrowth::PagedGrowth(void)
	{
		m_CheckToSame = false;
		m_CheckToOthers = false;
		m_Sort = false;
		m_AlphaRef = 0.4f;
		m_AlphaRefOverHigh = 0.4f;
		m_AlphaRefOverLow = 0.4f;
		//This is how many patches that should be visible from camera position in view direction
		m_VisGridSize = 8;
	}

	PagedGrowth::~PagedGrowth(void)
	{

	}

	bool PagedGrowth::Load(const std::string &wst_file)
	{
		TiXmlDocument     *xmlDoc = new TiXmlDocument(wst_file.c_str());
		if (!xmlDoc->LoadFile())
		{
			// Fatal error, cannot load
			printf("PagedGrowth::Load() - Couldn't load xmlfile: %s", wst_file.c_str());
			return 0;
		}
		TiXmlElement *top = 0;
		top = xmlDoc->FirstChildElement("underGrowth");
		if(!top) 
		{
			top = xmlDoc->FirstChildElement("overGrowth");
			if(!top) return 0;

		}
		m_Type = top->Value();

		if(!LoadXML(top)) return false;
		xmlDoc->Clear();

		// Delete our allocated document and return success ;)
		delete xmlDoc;
		return 1;
	}

	int PagedGrowth::LoadXML(TiXmlElement   *xmlElem)
	{
		std::string type = xmlElem->Value();
		if(type == "overGrowth") m_OverGrowth = true;
		else if(type == "underGrowth") m_OverGrowth = false;
		else printf("Unknown growth type: %s",type.c_str());

		m_MaterialMapFilename = xmlElem->Attribute("materialMapFilename");
		m_MaterialMapSideSize = atoi(xmlElem->Attribute("materialMapSideSize"));
		m_Viewdistance = static_cast<float>(atof(xmlElem->Attribute("viewdistance")));

		TiXmlElement *materials = 0;
		materials = xmlElem->FirstChildElement("materials");
		if(!materials) return 0;


		TiXmlElement *mat_elem = 0;
		int nInputCount = 0;

		mat_elem = materials->FirstChildElement();
		// Loop through each alien in the level

		while(mat_elem)
		{
			GrowthMaterial material;
			material.Name = mat_elem->Value();

			GrowthMaterialType mat_type;

			TiXmlElement *mat_types_elem = mat_elem->FirstChildElement("types");
			TiXmlElement *mat_type_elem = mat_types_elem->FirstChildElement();
			while(mat_type_elem)
			{
				mat_type.GeometryName = mat_type_elem->Attribute("geometryName");
				mat_type.GeometryName = mat_type.GeometryName;

				mat_type.MinRadiusDistToEquals = static_cast<float> (atof(mat_type_elem->Attribute("minRadiusDistToEquals")));
				mat_type.MinRadiusDistToOthers = static_cast<float> (atof(mat_type_elem->Attribute("minRadiusDistToOthers")));
				mat_type.NormalScale = static_cast<float> (atof(mat_type_elem->Attribute("normalScale")));
				mat_type.Probability = static_cast<float> (atof(mat_type_elem->Attribute("probability")));
				float min_s = static_cast<float> (atof(mat_type_elem->Attribute("min_scale")));
				float max_s = static_cast<float> (atof(mat_type_elem->Attribute("max_scale")));
				mat_type.Scale.set(min_s,max_s);
				material.MaterialTypes.push_back(mat_type);
				mat_type_elem = mat_type_elem->NextSiblingElement();
			}
			if(material.MaterialTypes.size() > 0) m_Materials.push_back(material);
			mat_elem = mat_elem->NextSiblingElement();
		}
		return 1;
	}

	int PagedGrowth::Init()
	{
		m_NameToId["default"] = 0;
		m_NameToId["water"] = 1;
		m_NameToId["dryGrass"] = 2;
		m_NameToId["juicyGrass"] = 3;
		m_NameToId["dryDirt"] = 4;
		m_NameToId["wetDirt"] = 5;
		m_NameToId["mud"] = 6;
		m_NameToId["deathMaterial"] = 7;
		m_NameToId["gravel"] = 8;
		m_NameToId["frozenGround"] = 9;
		m_NameToId["drySand"] = 10;
		m_NameToId["wetSand"] = 11;
		m_NameToId["rock"] = 12;
		m_NameToId["sandRoad"] = 13;
		m_NameToId["dirtRoad"] = 14;
		m_NameToId["pavedRoad"] = 15;
		m_NameToId["water"] = 16;

		//Initialize meshes and ids etc.
		for(int i = 0;  i< m_Materials.size(); i++)
		{
			GrowthMaterial* mat = &m_Materials[i];

			//map name to Id, faster lookup when adding objects
			if(m_NameToId.find(mat->Name) != m_NameToId.end())
			{
				mat->Id = m_NameToId[mat->Name];
			}
			else 
			{
				mat->Id = -1;
			}

			//Load all meshes for this material
			for(int j = 0; j < mat->MaterialTypes.size(); j++)
			{
				GrowthMaterialType* mt = &mat->MaterialTypes[j];
				/*if(!(mt->Geometry = (IGeometry*) Root::Get().GetGeometryTemplateManager()->CreateFromTemplate(mt->GeometryName)))
				{
					Log::Warning("GrowthImport::Load - Failed to load geometry %s",mt->GeometryName.c_str());
				}

				//assert(mt->Geometry);
				if(mt->Geometry) mt->Geometry->Init(NULL);

				//Check for high lod geometry
				mt->HighLodGeometry = NULL;
				std::string type = mt->GeometryName.substr(mt->GeometryName.length()-3,mt->GeometryName.length()-1);

				if(type == "_m2")
				{
					//Try to load m1
					std::string high_lod_name = mt->GeometryName.substr(0,mt->GeometryName.length()-3) + "_m1";
					if(mt->HighLodGeometry = (IGeometry*) Root::Get().GetGeometryTemplateManager()->CreateFromTemplate(high_lod_name))
						mt->HighLodGeometry->Init(NULL);
				}*/
			}
		}

		//How many random attempts are we going to do in patch (Max objects of each type)
		if(m_OverGrowth) m_SamplesInPatch = 20;
		else m_SamplesInPatch = 10;

		//int steps_in_patch = 3;
		//if(!ResourceAdmin::Get().GetFullPath(m_MaterialMapFilename, m_MaterialMapFilename)) Log::Error("Failed to load growth map");
		//m_GroundCover.Load(m_MaterialMapFilename,8); //4bit?
		float patch_size = m_Viewdistance /m_VisGridSize;
		float terrain_size = 200;//Root::GetPtr()->GetLevel()->GetTerrain()->GetSizeZ();
		//float step_size = patch_size/float(steps_in_patch);
		m_NumPatches = terrain_size/patch_size;

		float max_h = 100;//Root::GetPtr()->GetLevel()->GetMaxTerrainHeight();
		float min_h = 0;//Root::GetPtr()->GetLevel()->GetMinTerrainHeight();
		m_InvTerrainSize = 1.0f/terrain_size;

		float patch_rad = sqrt(patch_size*patch_size);
		//float rad = (max_h - min_h)/2;
		//patch_rad = Math::Max(sqrt(rad*rad),patch_rad);

		float inv_max_rand_mult_patch_size = (1.0f/((float)RAND_MAX))*patch_size;

		for(int i = 0 ; i < m_NumPatches; i++)
		{
			printf("Creating vegetation patch %d of %d",i*m_NumPatches,m_NumPatches*m_NumPatches);
			for(int j = 0 ; j < m_NumPatches; j++)
			{
				GrowthPatch patch;
				patch.Pos.x = i*patch_size;
				patch.Pos.y = 0;
				patch.Pos.z = -j*patch_size;

				//Make bounding box for culling
				patch.BoundingBox._min = osg::Vec3(patch.Pos.x,min_h,patch.Pos.z - patch_size);
				patch.BoundingBox._max = osg::Vec3(patch.Pos.x + patch_size,max_h,patch.Pos.z);

				//Make bounding sphere for culling, assume width is greater then height difference

				patch.BoudingSphere._center = osg::Vec3(patch.Pos.x() + patch_size/2, (min_h + max_h)/2,patch.Pos.x() - patch_size/2);
				patch.BoudingSphere._radius = patch_rad;

				for(int k = 0; k < m_SamplesInPatch; k++)
				{
					for(int l = 0;  l < m_Materials.size(); l++)
					{
						GrowthMaterial* mat = &m_Materials[l];
						for(int m = 0; m < mat->MaterialTypes.size(); m++)
						{
							GrowthMaterialType* gmat = &mat->MaterialTypes[m];

							//if(gmat->Geometry)
							{
								float xvalue = patch.Pos.x() + float(rand())*inv_max_rand_mult_patch_size;
								float zvalue = patch.Pos.z() - float(rand())*inv_max_rand_mult_patch_size;
								AddToPatch(xvalue,zvalue,&patch,mat,gmat);
							}
						}
					}
				}

				//Scramble position inside patch
				/*for(int k = 0; k < steps_in_patch*steps_in_patch; k++)
				{
				float xvalue = patch.Pos.x + float(rand())/(RAND_MAX)*patch_size;
				float zvalue = patch.Pos.z - float(rand())/(RAND_MAX)*patch_size;
				AddToPatch(xvalue,zvalue,&patch);
				}*/



				/*for(int x = 0; x < steps_in_patch; x++)
				{
				for(int z = 0; z < steps_in_patch; z++)
				{
				float pos_x = patch.Pos.x + x*step_size;
				float pos_z = patch.Pos.z + -z*step_size;
				AddToPatch(pos_x,pos_z,&patch);
				}
				}*/
				m_PatchVector.push_back(patch);
			}
		}
		return true;

	}


	void PagedGrowth::AddToPatch(float pos_x,float pos_z,GrowthPatch *patch,GrowthMaterial* mat, GrowthMaterialType* gmat)
	{
		bool has_data = false;
		osg::Vec3 pos,color,normal;
		float y;
		float u = pos_x*m_InvTerrainSize;
		float v = -pos_z*m_InvTerrainSize;

		if(u < 0 || v < 0 || u > 1 || v > 1) return;

		float fx = u*(m_GroundCover.m_Height-1);
		float fy = v*(m_GroundCover.m_Height-1);

		unsigned int px = fx;
		unsigned int py = fy;

		if((fx - px) > 0.5) px++;
		if((fy - py) > 0.5) py++;


		unsigned int index = m_GroundCover.m_Height*py + px;
		//unsigned int index = GetMapIndex(pos_x,pos_z);
		if(index >= 0)
		{

			int id = m_GroundCover.m_Data[index];

			//for(int i = 0;  i< m_Materials.size(); i++)
			{
				//GrowthMaterial* mat = &m_Materials[i];
				//srand();
				if(mat->Id == id)
				{
					//for(int j = 0; j < mat->MaterialTypes.size(); j++)
					{
						//GrowthMaterialType* gmat = &mat->MaterialTypes[j];
						float rvalue = float(rand())/(RAND_MAX); 
						//add object to patch
						if(rvalue <	gmat->Probability)
						{
							if(!has_data)
							{
								Root::GetPtr()->GetLevel()->GetTerrain()->GetAltitudeAndNormal(pos_x,pos_z,y,normal);
								pos = Vec3(pos_x,y,pos_z);
								color = Root::GetPtr()->GetLevel()->GetTerrainColor(pos_x,pos_z);
								has_data = true;

							}
							GrowthPatchObjectVector *pov;
							pov =  &patch->ObjectMap[gmat->GeometryName];
							//check for min distances to same

							if(m_CheckToSame)
							{
								for(int k = 0; k < pov->size(); k++)
								{
									GrowthPatchObject* po2 = &pov->at(k);
									float t1 = po2->Transform.m_Data[3][0]  - pos_x;//pos.x;
									float t2 = po2->Transform.m_Data[3][2]  - pos_z;//pos.z;
									float sqdist = t1*t1+t2*t2;
									if(sqdist < gmat->MinRadiusDistToEquals*gmat->MinRadiusDistToEquals) 
										return;
								}
							}
							//check for min distances to others

							if(m_CheckToOthers)
							{
								GrowthObjectMap::iterator iter;

								for(iter = patch->ObjectMap.begin();iter != patch->ObjectMap.end(); iter++)
								{
									GrowthPatchObjectVector *pov2 = &iter->second;

									if(pov == pov2) continue;
									for(int k = 0; k < pov2->size(); k++)
									{
										GrowthPatchObject* po2 = &pov2->at(k);
										float t1 = po2->Transform.m_Data[3][0] - pos_x;//pos.x;
										float t2 = po2->Transform.m_Data[3][2] - pos_z;//pos.z;
										float sqdist = t1*t1+t2*t2;
										if(sqdist < gmat->MinRadiusDistToOthers*gmat->MinRadiusDistToOthers) 
											return;
									}
								}
							}


							GrowthPatchObject po;
							//po.Pos = pos;
							po.Color = color;
							po.MaterialType = gmat;

							Vec3 up = normal*gmat->NormalScale + Vec3(0,1,0)*(1-gmat->NormalScale);
							up.Normalize();

							Vec3 right(1,0,0);
							Vec3 dir = Math::Cross(up,right);
							dir.Normalize();

							po.Transform.Identity();

							po.Transform.m_Data2[0] = right.x;
							po.Transform.m_Data2[1] = right.y;
							po.Transform.m_Data2[2] = right.z;

							po.Transform.m_Data2[4] = up.x;
							po.Transform.m_Data2[5] = up.y;
							po.Transform.m_Data2[6] = up.z;

							po.Transform.m_Data2[8] = dir.x;
							po.Transform.m_Data2[9] = dir.y;
							po.Transform.m_Data2[10] = dir.z;

							po.Transform.m_Data[3][0] = pos.x;
							po.Transform.m_Data[3][1] = pos.y;
							po.Transform.m_Data[3][2] = pos.z;

							float scale = po.MaterialType->Scale.GetValue();

							po.Transform.m_Data[0][0] *= scale;
							po.Transform.m_Data[1][1] *= scale;
							po.Transform.m_Data[2][2] *= scale;


							StandardMesh* mesh = NULL;
							mesh = (StandardMesh*)gmat->Geometry;
							
							if(mesh->GetMesh())
								po.BoudingSphere = mesh->GetMesh()->m_Mesh.GetBoundingSphere();
							po.BoudingSphere.m_Pos += pos;

							patch->ObjectMap[gmat->GeometryName].push_back(po);



						}
					}
				}	
			}
		}
	}

	bool DistComp(const GrowthPatchObject*& o1,const GrowthPatchObject*& o2)
	{
		return o1->Dist > o2->Dist;
	}

	void PagedGrowth::Render(ICameraNode* camera)
	{
		//Get center patch
	/*	m_SetupMeshCalls = 0;
		m_DrawMeshCalls = 0;
		float patch_size = m_Viewdistance /m_VisGridSize;
		float terrain_size = Root::GetPtr()->GetLevel()->m_Terrain->GetSize();
		Vec3 cam_pos = camera->GetAbsPos();

		int center_index_x = cam_pos.x/terrain_size*m_NumPatches;
		int center_index_z = -cam_pos.z/terrain_size*m_NumPatches;
		//render surrounding patches
		for(int i = -m_VisGridSize; i < m_VisGridSize+1; i++)
		{
			for(int j = -m_VisGridSize; j < m_VisGridSize+1; j++)
			{
				int ix = center_index_x + i;
				int iz = center_index_z + j;

				//boundary check
				if((ix >= 0 && ix <  m_NumPatches) &&
					(iz >= 0 && iz <  m_NumPatches))
				{
					PushPatch(camera,&m_PatchVector[iz+ix*m_NumPatches]);
				}
			}
		}

		//Render!!!
		if(!m_OverGrowth)
		{
			float env_color[] = {1,1,1,1};
			glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,env_color);
			glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_BLEND);
			//glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
			
		}

		Root::GetPtr()->m_ActiveRenderer->SetCulling(RenderSystem::CULL_CLOCKWISE);

		GrowthRenderMap::iterator iter;
		for(iter = m_RenderMap.begin(); iter != m_RenderMap.end(); iter++)
		{
			//Setup  mesh
			if(iter->second.size() > 0)
			{
				GrowthPatchObjectPointerVector* pov = &iter->second;
				if(m_Sort)
				{
					std::sort(pov->begin(),pov->end(),DistComp);
				}
				m_SetupMeshCalls++;
				if(!m_OverGrowth) RenderUnderGrowth(cam,pov);
				else RenderOverGrowth(cam,pov);

				pov->clear();
			}
		}
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		//Root::GetPtr()->m_ActiveRenderer->SetCulling(RenderSystem::CULL_ANTICLOCKWISE);*/

		/*Font::DebugPrint("Mesh setup calls %d",m_SetupMeshCalls);
		Font::DebugPrint("Draw Mesh calls %d",m_DrawMeshCalls);
		if(m_Sort) Font::DebugPrint("sort render queue (press 2 to disable)");
		else Font::DebugPrint("press 1 to enable sort render queue");

		Font::DebugPrint("alpha ref %.3f",m_AlphaRef);

		if(GetAsyncKeyState('1')) m_Sort = true;
		if(GetAsyncKeyState('2')) m_Sort = false;
		if(GetAsyncKeyState('3')) m_AlphaRef += 0.01;
		if(GetAsyncKeyState('4')) m_AlphaRef -= 0.01;

		if(GetAsyncKeyState('5')) m_AlphaRefOverHigh += 0.01;
		if(GetAsyncKeyState('6')) m_AlphaRefOverHigh -= 0.01;

		if(GetAsyncKeyState('7')) m_AlphaRefOverLow += 0.01;
		if(GetAsyncKeyState('8')) m_AlphaRefOverLow -= 0.01;*/
	}

	void PagedGrowth::RenderUnderGrowth(ICameraNode* cam,GrowthPatchObjectPointerVector* pov)
	{
		/*GrowthPatchObjectPointerVector::iterator pov_iter;
		GrowthPatchObject* obj = pov->front();
		//assume one lod level and one mesh part

		StandardMesh* mesh = (StandardMesh*)obj->MaterialType->Geometry;
		if(mesh == NULL) return;
		if(mesh->GetMesh() == NULL) return;

		LODMesh *lm = mesh->GetMesh()->m_Mesh.GetLODMesh(0);
		MeshPart* mp = &lm->MeshPartVector[0];

		//mesh->GetMesh()->m_Mesh.SetupMaterial(mp->Material);
		glDisable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);

		Root::GetPtr()->m_ActiveRenderer->SetSceneBlending(SBF_SOURCE_ALPHA,SBF_ONE_MINUS_SOURCE_ALPHA);

		GLIndexBuffer* glib = (GLIndexBuffer*) mp->IndexBuffer;
		GLVertexBuffer* glvb = (GLVertexBuffer*) mp->VertexBuffer;
		if(glBindBufferARB) glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, glib->m_BufferID);
		((GLRenderSystem*) Root::GetPtr()->m_ActiveRenderer)->SetupVertexBuffer(glvb);
		Root::GetPtr()->m_ActiveRenderer->SetTexture(0,true,mp->Material->m_DiffuseMap);
		//Root::GetPtr()->m_ActiveRenderer->SetSurfaceParams(mp->Material->m_AmbientColor,mp->Material->m_DiffuseColor, mp->Material->m_SpecularColor,mp->Material->m_EmissiveColor, mp->Material->m_SpecularPower);

		float start_fade = 4*m_Viewdistance/5.0f;
		float fade_dist = m_Viewdistance - start_fade;
		for(pov_iter = pov->begin();  pov_iter != pov->end(); pov_iter++)
		{
			obj = *pov_iter;
			float l = obj->Dist;
			float alpha = 1;

			if(l > start_fade) alpha = 1 - (l - start_fade)/fade_dist;

			glPushMatrix();
			glMultMatrixf(&obj->Transform.m_Data2[0]);
			//					glTranslatef(obj->Pos.x,obj->Pos.y,obj->Pos.z);
			//float env_color[] = {obj->Color.x,obj->Color.y,obj->Color.z,1};
			//glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,env_color);
			//glColor4f(0,0,0,alpha);
			glColor4f(0.5*obj->Color.x,0.5*obj->Color.y,0.5*obj->Color.z,alpha);
			glDrawElements(GL_TRIANGLES, glib->m_NumElem, GL_UNSIGNED_INT, glib->m_List);
			//Root::GetPtr()->m_ActiveRenderer->RenderBuffer(mp->IndexBuffer,mp->VertexBuffer);
			glPopMatrix();
			m_DrawMeshCalls++;
		}*/
	}

	void PagedGrowth::RenderOverGrowth(ICameraNode* cam,GrowthPatchObjectPointerVector* pov)
	{
		/*GrowthPatchObjectPointerVector::iterator pov_iter;
		GrowthPatchObject* obj = pov->front();
		//assume one lod level and one mesh part

		//mesh->GetMesh()->m_Mesh.SetupMaterial(mp->Material);
		glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
		glEnable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
		glEnable(GL_BLEND);
		glDepthMask(true);

		

		glAlphaFunc(GL_GREATER, m_AlphaRef);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
		glBlendColor(1,1,1,1);
		glColor4f(1,1,1,1);


		StandardMesh* mesh = (StandardMesh*)obj->MaterialType->Geometry;
		if(mesh == NULL) return;
		if(mesh->GetMesh() == NULL) return;
		LODMesh *lm = mesh->GetMesh()->m_Mesh.GetLODMesh(0);
		MeshPart* mp = &lm->MeshPartVector[0];
		GLIndexBuffer* glib = (GLIndexBuffer*) mp->IndexBuffer;
		GLVertexBuffer* glvb = (GLVertexBuffer*) mp->VertexBuffer;
		if(glBindBufferARB) glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, glib->m_BufferID);
		((GLRenderSystem*) Root::GetPtr()->m_ActiveRenderer)->SetupVertexBuffer(glvb);
		Root::GetPtr()->m_ActiveRenderer->SetTexture(0,true,mp->Material->m_DiffuseMap);
		Root::GetPtr()->m_ActiveRenderer->SetSurfaceParams(mp->Material->m_AmbientColor,mp->Material->m_DiffuseColor, mp->Material->m_SpecularColor,mp->Material->m_EmissiveColor, mp->Material->m_SpecularPower);

		

		float start_fade;
		float fade_span;
		float fade_in,fade_out;

		if(obj->MaterialType->HighLodGeometry)
		{
			start_fade = m_Viewdistance/6.0f;
			fade_span = 10;
		}
		else
		{
			start_fade = m_Viewdistance - 2*m_Viewdistance/5.0f;
			fade_span = m_Viewdistance - start_fade;
		}

		//glDepthMask(false);

		for(pov_iter = pov->begin();  pov_iter != pov->end(); pov_iter++)
		{
			obj = *pov_iter;
			fade_in = FadeIn(obj->Dist, start_fade, fade_span);
			fade_out = FadeOut(obj->Dist, start_fade+fade_span, fade_span);
			obj->Alpha = fade_out;

			if(fade_in > 0)
			{
			
				glPushMatrix();
				glMultMatrixf(&obj->Transform.m_Data2[0]);
				//glColor4f(1,1,1,fade_in);
				//glBlendColor(1,1,1,fade_in);
				//glAlphaFunc(GL_GREATER, m_AlphaRef + (1-m_AlphaRef)*(1-fade_in));
				glAlphaFunc(GL_GREATER, 1 - (1-m_AlphaRefOverLow)*fade_in);
				glDrawElements(GL_TRIANGLES, glib->m_NumElem, GL_UNSIGNED_INT, glib->m_List);
				glPopMatrix();
				m_DrawMeshCalls++;
			}
		}
		
		glDepthMask(true);
		if(obj->MaterialType->HighLodGeometry)
		{
			StandardMesh* mesh = (StandardMesh*)obj->MaterialType->HighLodGeometry;

			LODMesh *lm = mesh->GetMesh()->m_Mesh.GetLODMesh(0);
			for(int i = 0; i < lm->MeshPartVector.size(); i++)
			{
				MeshPart* mp = &lm->MeshPartVector[i];

				//mesh->GetMesh()->m_Mesh.SetupMaterial(mp->Material);
				//glDisable(GL_LIGHTING);
				glDisable(GL_COLOR_MATERIAL);

				GLIndexBuffer* glib = (GLIndexBuffer*) mp->IndexBuffer;
				GLVertexBuffer* glvb = (GLVertexBuffer*) mp->VertexBuffer;
				if(glBindBufferARB) glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, glib->m_BufferID);
				((GLRenderSystem*) Root::GetPtr()->m_ActiveRenderer)->SetupVertexBuffer(glvb);
				Root::GetPtr()->m_ActiveRenderer->SetTexture(0,true,mp->Material->m_DiffuseMap);
				Root::GetPtr()->m_ActiveRenderer->SetSurfaceParams(mp->Material->m_AmbientColor,mp->Material->m_DiffuseColor, mp->Material->m_SpecularColor,mp->Material->m_EmissiveColor, mp->Material->m_SpecularPower);

				for(pov_iter = pov->begin();  pov_iter != pov->end(); pov_iter++)
				{

					obj = *pov_iter;
					//fade_in = FadeOut(obj->Dist, start_fade, fade_span);


					//FaderOut(obj->Dist, start_fade, fade_span,fade_out);

					if(obj->Alpha > 0) 
					{
						glPushMatrix();
						glMultMatrixf(&obj->Transform.m_Data2[0]);
						glAlphaFunc(GL_GREATER, m_AlphaRefOverHigh + (1-m_AlphaRefOverHigh)*(1 - obj->Alpha));
						glDrawElements(GL_TRIANGLES, glib->m_NumElem, GL_UNSIGNED_INT, glib->m_List);
						glPopMatrix();
						m_DrawMeshCalls++;
					}
				}
			}
		}
		glAlphaFunc(GL_GREATER, 0.1);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/
	}

	void PagedGrowth::PushPatch(ICameraNode* cam, GrowthPatch* patch)
	{
		//bounding box check
		if(cam->BoxInFrustum(patch->BoundingBox))
		{
			//patch->BoundingBox.Draw();
			//Do distance check also......dont have the motivation right now
			GrowthObjectMap::iterator iter;
			for(iter = patch->ObjectMap.begin(); iter != patch->ObjectMap.end(); iter++)
			{
				//put objects in right queue
				if(iter->second.size() > 0)
				{
					GrowthPatchObject* obj = &iter->second[0];
					GrowthPatchObjectPointerVector* render_buffer;
					render_buffer = &m_RenderMap[obj->MaterialType->GeometryName];

					for(int i = 0; i < iter->second.size(); i++)
					{
						obj = &iter->second[i];
						float square_dist = (cam->GetPosition() - obj->Transform.GetTranslation()).SquaredLength();
						if(square_dist < m_Viewdistance*m_Viewdistance) 
						{
							if(cam->SphereInFrustum(obj->BoudingSphere))
							{
								
								//obj->Dist = sqrt(square_dist);
								obj->Dist = 1.0f/Math::InvSqrt(square_dist);
								render_buffer->push_back(obj);
							}
						}
					}
				}
			}
		}
	}

	float PagedGrowth::FadeOut(float dist, float start_fade, float span)
	{
		float v1 = dist-start_fade;
		if(v1 < 0) return 1;
		v1 = v1/span;
		if(v1 > 1) return 0;
		else return 1-v1;
	}

	float PagedGrowth::FadeIn(float dist, float start_fade, float span)
	{
		float v1 = dist-start_fade;
		if(v1 < 0) return 0;
		v1 = v1/span;
		if(v1 > 1) return 1;
		else return v1;
	}
}
