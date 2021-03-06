#include <SDFInputDataItem.h>
#include <mainWindow.h>

#include <BALL/QSAR/exception.h>
#include <exception.h>
#include <inputDataDialog.h>

#include <QtGui/QDialog>
#include <QtGui/QDrag>
#include <QtCore/QMimeData>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <BALL/VIEW/KERNEL/iconLoader.h>

using namespace BALL::QSAR;
using namespace BALL::Exception;
using namespace BALL::QSAR::Exception;

namespace BALL
{
	namespace VIEW
	{
		SDFInputDataItem::SDFInputDataItem(QString filename,std::multiset<int> act, bool cdv, bool crv, DataItemView* view)
		 : InputDataItem(filename, cdv, crv, view)
		{
			//set pixmap
			setPixmap(findPixmap("sdf_icon"));

			QStringList list = filename_.split("/");
			setName(list[list.size()-1]);
			
			// data_ is initialized by base class constructor!
			
			if(act.size()==0) activity_values_.clear();
			else activity_values_ = act;
		}


		SDFInputDataItem::SDFInputDataItem(QString filename, DataItemView* view):
			InputDataItem(filename, false, false, view)
		{
			setPixmap(findPixmap("sdf_icon"));

			QStringList list = filename_.split("/");
			setName(list[list.size()-1]);
			activity_values_.clear();

			// data_ is initialized by base class constructor!
		}

		SDFInputDataItem::~SDFInputDataItem()
		{
			if (view_->name == "view")
			{
				//if the item was connected to others, delete it from its respective pipeline
				if (!removeDisconnectedItem())
				{
					removeFromPipeline();
				}
			}
			
			// base class destructor will delete the connected QSARData object,
			// so make sure that connected InputDataItems know that there is no QSARData object any longer!
			if(!append_)
			{
				for(list<CSVInputDataItem*>::iterator it=additional_descriptors_.begin();it!=additional_descriptors_.end();it++)
				{
					(*it)->setData(NULL);
				}
			}
		}

		SDFInputDataItem::SDFInputDataItem(SDFInputDataItem& item):
			InputDataItem(filename_, center_data_, center_y_ ,item.view_)
		{
			data_ = new QSARData(*item.data_) ;
 			activity_values_ = item.activity_values_;
			name_ = item.name_;
			setPixmap(item.pixmap());
		}

		bool SDFInputDataItem::execute()
		{
			if(done_) return 0; // do nothing twice...
			
			string st = filename_.toStdString();
			try
			{
				data_->setDataFolder(view_->data_scene->main_window->getDataDirectory().c_str());
				data_->readSDFile(st.c_str(), activity_values_, use_SD_properties_, nonnumeric_class_names_);
			}
			catch(WrongFileFormat)
			{
				throw(InvalidInputDataItem(__FILE__,__LINE__));
			}
			///this one occurs if there were some properties missing
			catch(PropertyError)
			{
				throw(InvalidInputDataItem(__FILE__,__LINE__));
			}

			/// center data only if desired by the user and if there are no additional descriptors being read from a csv-file. In the latter case, centering is left to be done by the CSVInputDataItem.
			if (additional_descriptors_.size()==0 && center_data_)
			{
				data_->centerData(center_y_);
			}
			
			done_ = 1;
			return 1;
		}


		bool SDFInputDataItem::checkForDiscreteY()
		{
			if(checked_for_discrete_y_) return discrete_y_;
			
			try
			{
				if(done_) // if data has already been read, check within data-structure
				{
					discrete_y_=data_->checkforDiscreteY();
				}
				else // if data has not been read, check within file
				{
					discrete_y_=data_->checkforDiscreteY(filename_.toStdString().c_str(), activity_values_);
				}
			}
			catch(BALL::Exception::GeneralException e)
			{	
				// e.g. input file has not been found
				QMessageBox::critical(view_,e.getName(),e.getMessage());
				return 0;
			}
			
			checked_for_discrete_y_ = 1;
			return discrete_y_;
		}

		std::multiset<int> SDFInputDataItem::activityValues()
		{
			return activity_values_;
		}

		void SDFInputDataItem::setActivityValues(std::multiset<int> act)
		{
			activity_values_ = act;
		}


		void SDFInputDataItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* /*event*/)
		{
			InputDataDialog inputDataDialog(this);
			inputDataDialog.exec();
		}

		// called by DataItemScene when dropping a CSVInputDataItem onto this item
		void SDFInputDataItem::appendCSVDescriptors(CSVInputDataItem* item)
		{
			/// make sure that centering is ONLY done by the last CSVInputDataItem!
			if(additional_descriptors_.size()!=0)
			{
				CSVInputDataItem* csv_item = additional_descriptors_.back();
				csv_item->setCenterDataFlag(0);
				csv_item->setCenterResponseFlag(0);
			}
			additional_descriptors_.push_back(item);
		}


		list<CSVInputDataItem*>* SDFInputDataItem::getConnectedCSVItems()
		{
			return &additional_descriptors_;
		}

		void SDFInputDataItem::addToPipeline()
		{
			view_->data_scene->main_window->sdf_input_pipeline_.insert(this);
			view_->data_scene->main_window->all_items_pipeline_.insert(this);
		}

		void SDFInputDataItem::removeFromPipeline()
		{
			view_->data_scene->main_window->sdf_input_pipeline_.erase(this);
			view_->data_scene->main_window->all_items_pipeline_.erase(this);
		}

		void SDFInputDataItem::replaceItem(InputDataItem* old_item)
		{	
			SDFInputDataItem* old_sdf = dynamic_cast<SDFInputDataItem*>(old_item); 
			if(old_sdf)
			{
				// delete all features that were read from CSV-files appended to the old SDF-item
				for(list<CSVInputDataItem*>::iterator csv_it=old_sdf->additional_descriptors_.begin(); csv_it!=old_sdf->additional_descriptors_.end(); csv_it++)
				{
					delete *csv_it;
				}
				
				// put the new item into the the pipeline at the same position
				Pipeline<SDFInputDataItem*>::iterator sdf_it=view_->data_scene->main_window->sdf_input_pipeline_.find(old_sdf);
				if(sdf_it!=view_->data_scene->main_window->sdf_input_pipeline_.end())
				{
					view_->data_scene->main_window->sdf_input_pipeline_.insert(sdf_it,this);
				}
				else view_->data_scene->main_window->sdf_input_pipeline_.insert(this);
			}	
			else view_->data_scene->main_window->sdf_input_pipeline_.insert(this);
			
			Pipeline<DataItem*>::iterator it = view_->data_scene->main_window->all_items_pipeline_.find(old_item);
			if(it!=view_->data_scene->main_window->all_items_pipeline_.end())
			{
				view_->data_scene->main_window->all_items_pipeline_.insert(it,this);
			}
			else
			{
				view_->data_scene->main_window->all_items_pipeline_.insert(this);
			}
			
			transferEdges(old_item);  // steal the old item's edges
			delete old_item; // then kill the old item
				
			// finially, make sure that the entire pipeline created by use of this input is reset
			change();
		}

		void SDFInputDataItem::useSDProperties(bool b)
		{
			use_SD_properties_ = b;
		}

		bool SDFInputDataItem::useSDProperties()
		{
			return use_SD_properties_;
		}

		void SDFInputDataItem::loadFromFile(String file)
		{
			InputDataItem::loadFromFile(file);
			
			// if dat-file was read sucessfully, make sure that it is not read again by connected CSV-items
			if(done_)
			{
				for(list<CSVInputDataItem*>::iterator csv_it=additional_descriptors_.begin(); csv_it!=additional_descriptors_.end(); csv_it++)
				{
					(*csv_it)->setDone(1);
				}
			}
		}
	}
}
