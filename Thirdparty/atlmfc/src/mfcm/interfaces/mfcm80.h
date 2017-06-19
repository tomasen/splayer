// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#using <mscorlib.dll>
#using <system.dll>


using namespace System;
using namespace System::ComponentModel;


namespace Microsoft{
	namespace VisualC{
		namespace MFC{
			public interface class IView
			{
				void OnInitialUpdate();
				void OnUpdate();
				void OnActivateView(bool activate);
				
			};
			//public enum UICheckState : byte { Unchecked, Checked, Indeterminate };
			public enum class UICheckState
			{
				[DefaultValue(Microsoft::VisualC::MFC::UICheckState::typeid, "Checked")]
				Unchecked, 
					Checked, 
					Indeterminate 
			};

			public interface class ICommandUI
			{
				void ContinueRouting();
				property bool  Enabled 
				{
					void set(bool value);
				}
				property UICheckState Check
				{	
					void set(UICheckState value);
				}
				property bool  Radio
				{
					void set(bool value);
				}
				property String^  Text
				{
					void set(String^ value);
				}
				property unsigned int  ID
				{
					unsigned int get();
				}
				property unsigned int  Index
				{
					unsigned int get();
				}				
			};

			public delegate void CommandHandler(unsigned int cmdID);
			public delegate void CommandUIHandler(unsigned int cmdID, ICommandUI^ cmdUI);

			public interface class ICommandSource
			{
				void AddCommandHandler(unsigned int cmdID, CommandHandler^ cmdHandler);
				void AddCommandRangeHandler(unsigned int cmdIDMin, unsigned int cmdIDMax, CommandHandler^ cmdHandler);
				void RemoveCommandHandler(unsigned int cmdID);
				void RemoveCommandRangeHandler(unsigned int cmdIDMin, unsigned int cmdIDMax);

				void AddCommandUIHandler(unsigned int cmdID, CommandUIHandler^ cmdUIHandler);
				void AddCommandRangeUIHandler(unsigned int cmdIDMin, unsigned int cmdIDMax, CommandUIHandler^ cmdUIHandler);
				void RemoveCommandUIHandler(unsigned int cmdID);
				void RemoveCommandRangeUIHandler(unsigned int cmdIDMin, unsigned int cmdIDMax);

				void PostCommand(unsigned int command );
				void SendCommand(unsigned int command);
			};

			public  interface class ICommandTarget
			{
				void Initialize(ICommandSource^ cmdSource);
			};


		}
	}
}