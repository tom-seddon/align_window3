#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <math.h>
#include <vector>
#include <stdint.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// <pre>
//
// +-------+-------+-------+
// |   1   |   2   |   3   |
// +-------+-------+-------+
//
// +-----------+-----------+
// |     4     |     6     |
// +-----------+-----------+
//
// +-----------------------+
// |           5           |
// +-----------------------+
//
// +---------------+
// |       7       |       
// +---------------+
//
//         +---------------+
//         |       9       |
//         +---------------+
//
// +---+ +---+ +---+ +---+
// |   | |   | |   | |   | 
// | 1 | |   | |   | |   |
// |   | | 4 | |   | |   |
// +---+ |   | |   | | 7 | +---+
// |   | |   | |   | |   | |   |
// | 2 | +---+ | 5 | |   | |   |
// |   | |   | |   | |   | |   |
// +---+ |   | |   | +---+ | 9 |
// |   | | 6 | |   |       |   |
// | 3 | |   | |   |       |   |
// |   | |   | |   |       |   |
// +---+ +---+ +---+       +---+
//
// (no, these are not very mnemonic...)

static LONG Lerp(LONG a, LONG b, double t) {
	return (LONG)(a + (b - a)*t);
}

static LONG GetX(const RECT &r, double t) {
	return r.left + (LONG)(t*(r.right - r.left));
}

static LONG GetY(const RECT &r, double t) {
	return r.top + (LONG)(t*(r.bottom - r.top));
}

static RECT GetXRect(const RECT &win, const RECT &work, double x0, double x1) {
	RECT result = win;

	result.left = GetX(work, x0);
	result.right = GetX(work, x1);

	return result;
}

static RECT GetYRect(const RECT &win, const RECT &work, double y0, double y1) {
	RECT result = win;

	result.top = GetY(work, y0);
	result.bottom = GetY(work, y1);

	return result;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	HWND h = GetForegroundWindow();
	if (!h)
		return 1;

	int argc;
	wchar_t **argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argc < 2)
		return 1;

	MONITORINFO mi;
	mi.cbSize = sizeof mi;
	GetMonitorInfo(MonitorFromWindow(h, MONITOR_DEFAULTTOPRIMARY), &mi);

	RECT old_wr;
	GetWindowRect(h, &old_wr);

	bool can_resize = !!(GetWindowLong(h, GWL_STYLE)&WS_THICKFRAME);

	RECT(*fn)(const RECT &, const RECT &, double, double) = nullptr;

	const wchar_t *arg = argv[1];
	if (arg[0] == L'x')
		fn = &GetXRect;
	else if (arg[0] == L'y')
		fn = &GetYRect;

	if (!fn)
		return 1;

	++arg;

	RECT new_wr;

	if (arg[0] == L'1') {
		new_wr = (*fn)(old_wr, mi.rcWork, 0 / 3.0, 1 / 3.0);
	} else if (arg[0] == L'2') {
		new_wr = (*fn)(old_wr, mi.rcWork, 1 / 3.0, 2 / 3.0);
	} else if (arg[0] == L'3') {
		new_wr = (*fn)(old_wr, mi.rcWork, 2 / 3.0, 3 / 3.0);
	} else if (arg[0] == L'4') {
		new_wr = (*fn)(old_wr, mi.rcWork, 0 / 2.0, 1 / 2.0);
	} else if (arg[0] == L'5') {
		new_wr = (*fn)(old_wr, mi.rcWork, 0.0, 1.0);
	} else if (arg[0] == L'6') {
		new_wr = (*fn)(old_wr, mi.rcWork, 1 / 2.0, 2 / 2.0);
	} else if (arg[0] == L'7') {
		new_wr = (*fn)(old_wr, mi.rcWork, 0 / 3.0, 2 / 3.0);
	} else if (arg[0] == L'9') {
		new_wr = (*fn)(old_wr, mi.rcWork, 1 / 3.0, 3 / 3.0);
	} else
		return 1;

	// 	std::vector<RECT> rects;
	// 
	// 	if(_wcsicmp(arg,L"33")==0) {
	// 		for(int i=0;i<3;++i)
	// 			rects.push_back((*fn)(wr,mi.rcWork,i/3.0,(i+1)/3.0));
	// 	} else if(_wcsicmp(arg,L"50")==0) {
	// 		for(int i=0;i<2;++i)
	// 			rects.push_back((*fn)(wr,mi.rcWork,i/2.0,(i+1)/2.0));
	// 	} else if(_wcsicmp(arg,L"66")==0) {
	// 		for(int i=0;i<2;++i)
	// 			rects.push_back((*fn)(wr,mi.rcWork,i/3.0,(i+2)/3.0));
	// 	} else if(_wcsicmp(arg,L"100")==0) {
	// 		rects.push_back((*fn)(wr,mi.rcWork,0.0,1.0));
	// 	}
	// 
	// 	if(rects.empty())
	// 		return 1;
	// 
	// 	size_t best_idx=0;
	// 	uint64_t best_score=UINT64_MAX;
	// 
	// 	for(size_t i=0;i<rects.size();++i) {
	// 		const RECT *r=&rects[i];
	// 
	// 		RECT d;
	// 
	// 		d.left=r->left-wr.left;
	// 		d.top=r->top-wr.top;
	// 		d.right=r->right-wr.right;
	// 		d.bottom=r->bottom-wr.bottom;
	// 
	// 		uint64_t score=0;
	// 		score+=d.left*d.left;
	// 		score+=d.top*d.top;
	// 		score+=d.right*d.right;
	// 		score+=d.bottom*d.bottom;
	// 
	// 		if(score<best_score) {
	// 			best_score=score;
	// 			best_idx=i;
	// 		}
	// 	}
	// 
	// 	size_t next_idx;
	// 	if(best_score==0)
	// 		next_idx=(best_idx+1)%rects.size();
	// 	else
	// 		next_idx=best_idx;
	// 
	// 	wr=rects[next_idx];

	if (!EqualRect(&new_wr, &old_wr)) {
		if (EqualRect(&new_wr, &mi.rcWork)) {
			ShowWindow(h, SW_MAXIMIZE);
		} else {
			if (IsZoomed(h))
				ShowWindow(h, SW_RESTORE);

			UINT swp_flags = SWP_NOCOPYBITS | SWP_NOOWNERZORDER | SWP_NOZORDER;
			SetWindowPos(h, 0, new_wr.left, new_wr.top, new_wr.right - new_wr.left, new_wr.bottom - new_wr.top, swp_flags);
		}
	}

	return 0;
}
