//namespace ImGui
//{
//	struct IMGUI_API ImFileInfo
//	{
//		ImFileInfo(const char* name, int64_t size);
//		~ImFileInfo();
//
//		ImString Name;
//		int64_t Size;
//	};
//
//	struct IMGUI_API ImFileList
//	{
//		typedef ImVector<ImFileInfo> FileInfoArray;
//		FileInfoArray FileList;
//		int Pos;
//
//		ImFileList(const char* path = ".")
//			: Pos(0)
//		{
//			ChDir(path);
//		}
//
//		void ChDir(const char* path);
//		void Draw();
//	};
//
//} // namespace ImGui
