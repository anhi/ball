// -*- Mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//

#include <BALL/VIEW/DIALOGS/assignBondOrderConfigurationDialog.h>
#include <BALL/VIEW/KERNEL/common.h>
#include <BALL/VIEW/KERNEL/mainControl.h>
#include <BALL/SYSTEM/path.h>

#include <BALL/STRUCTURE/assignBondOrderProcessor.h>
#include <QtGui/qlineedit.h>
#include <QtGui/qradiobutton.h>
#include <QtGui/qcheckbox.h>
#include <QtGui/qpushbutton.h>
#include <QtGui/QFileDialog>

namespace BALL
{
	namespace VIEW
	{

		AssignBondOrderConfigurationDialog::AssignBondOrderConfigurationDialog(QWidget* parent, const char* name)
			:	QDialog(parent),
				Ui_AssignBondOrderConfigurationDialogData(),
				ModularWidget(name),
				PreferencesEntry()
		{
			setupUi(this);

			setINIFileSectionName("BONDORDERASSIGNER"); 
			setObjectName(name);
			// for preferences	
			registerWidgets_();
			// for actions, icons...
			ModularWidget::registerWidget(this);
			hide();

			// signals and slots connections
			connect( close_button, SIGNAL( clicked() ), this, SLOT( accept() ) );
			connect( cancel_button, SIGNAL( clicked() ), this, SLOT( reject() ) );
			connect( reset_button, SIGNAL( clicked() ), this, SLOT( resetOptions() ) );
			connect( browse_button, SIGNAL( clicked() ), this, SLOT( browseParameterFiles_() ) );	
			connect( penalty_balance_slider, SIGNAL( valueChanged(int) ), this, SLOT( balanceParameterChanged_() ) );

			connect(overwrite_singleBO_box, SIGNAL(stateChanged(int)), this, SLOT(validateBOBoxes_()));
			connect(overwrite_doubleBO_box, SIGNAL(stateChanged(int)), this, SLOT(validateBOBoxes_()));
			connect(overwrite_tripleBO_box, SIGNAL(stateChanged(int)), this, SLOT(validateBOBoxes_()));
			connect(overwrite_selected_bonds_box, SIGNAL(stateChanged(int)), this, SLOT(validateBOBoxes_()));

			validateBOBoxes_();
		}

		AssignBondOrderConfigurationDialog::~AssignBondOrderConfigurationDialog()
			throw()
		{
		}

		void AssignBondOrderConfigurationDialog::balanceParameterChanged_()
		{
			atom_type_penalty_label->setText(String((int)(100 - penalty_balance_slider->value())).c_str());
			bond_length_penalty_label ->setText(String(penalty_balance_slider->value()).c_str());
		}

		void AssignBondOrderConfigurationDialog::validateBOBoxes_()
		{
			// if one of the bond orders is checked, the "selected" box must be
			// de-activated and vice versa
			bool    selected_checked = overwrite_selected_bonds_box->isChecked();
		
			/* // Version 1
			bool bond_orders_checked = (   overwrite_singleBO_box->isChecked() 
																	|| overwrite_doubleBO_box->isChecked()
																	|| overwrite_tripleBO_box->isChecked() );
			// this is just a safety check: it should never be possible to check
			// both kinds of boxes, but you never know... :-)
			if (bond_orders_checked && selected_checked)
			{
				bond_orders_checked = false;
				selected_checked    = false;
			}

			overwrite_singleBO_box->setDisabled(selected_checked);
			overwrite_doubleBO_box->setDisabled(selected_checked);
			overwrite_tripleBO_box->setDisabled(selected_checked);

			overwrite_selected_bonds_box->setDisabled(bond_orders_checked); */
			// Version 2
			if (selected_checked)
			{
				overwrite_singleBO_box->setChecked(!selected_checked);
				overwrite_doubleBO_box->setChecked(!selected_checked);
				overwrite_tripleBO_box->setChecked(!selected_checked);
			}
			overwrite_singleBO_box->setDisabled(selected_checked);
			overwrite_doubleBO_box->setDisabled(selected_checked);
			overwrite_tripleBO_box->setDisabled(selected_checked);
		
		}

		void AssignBondOrderConfigurationDialog::browseParameterFiles_()
		{
			// look up the full path of the parameter file
			Path p;
			String filename = p.find(ascii(parameter_file_edit->text()));

			if (filename == "")
			{
				filename = ascii(parameter_file_edit->text());
			}

			QString tmp = filename.c_str();
			QString result = QFileDialog::getOpenFileName(0, "Select the penalty parameter file", tmp, "*.xml");
			if (!result.isEmpty())
			{
				// store the new filename in the lineedit field
				parameter_file_edit->setText(result);
			}
		}

		void AssignBondOrderConfigurationDialog::reject()
		{
			hide();
			PreferencesEntry::restoreValues();
		}

		void AssignBondOrderConfigurationDialog::accept()
		{
			hide();
			PreferencesEntry::storeValues();
			QDialog::accept();
		}


		String AssignBondOrderConfigurationDialog::getValue_(const QCheckBox* box) const
		{
			if (box->isChecked()) return "true";
			else 									return "false";
		}

		float AssignBondOrderConfigurationDialog::getValue_(const QLineEdit* edit) const
			throw(Exception::InvalidFormat)
		{
			return ascii(edit->text()).toFloat();
		}

		void  AssignBondOrderConfigurationDialog::initializeWidget(MainControl&)
		{
		}

		void AssignBondOrderConfigurationDialog::resetOptions()
		{
			PreferencesEntry::restoreDefaultValues();
		}

	}//namespace VIEW
}//namespace BALL