#include <map>
#include <vector>
#include <cairo/cairo.h>
#include "LabelEngine.h"
#include "Regrouper.h"
#include "ReadInputO5m.h"
#include "Transform.h"
#include "drawlib/drawlibcairo.h"
#include "MapRender.h"

using namespace std;

typedef map<int, map<int, LabelsByImportance> > OrganisedLabelsMap;

///Main function for program
int main()
{

	CoastMap coastMap("fosm-coast-earth201507161012.bin");

	// ** Collect labels from off screen tiles
	OrganisedLabelsMap organisedLabelsMap;
	cairo_surface_t *offScreenSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 640, 640);

	for(int x=2034; x <= 2036; x++)
		for(int y=1373; y<= 1375; y++)
		{
			if(x == 2035 && y == 1374) continue;
			FeatureStore featureStore;
			ReadInputO5m(12, "../iridescent-testdata", x, y, featureStore);

			class DrawLibCairoPango drawlib(offScreenSurface);	
			class MapRender mapRender(&drawlib, x, y, 12, x, y, 12, "../iridescent-testdata");
			mapRender.SetCoastMap(coastMap);
			LabelsByImportance organisedLabels;
			
			mapRender.Render(12, featureStore, false, true, organisedLabels);

			OrganisedLabelsMap::iterator it = organisedLabelsMap.find(x);
			if(it == organisedLabelsMap.end())
				organisedLabelsMap[x] = map<int, LabelsByImportance>();
			map<int, LabelsByImportance> &col = organisedLabelsMap[x];
			col[y] = organisedLabels;
		}

	cairo_surface_destroy(offScreenSurface);

	// ** Render without labels and collect label info **

	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 640, 640);
	FeatureStore featureStore;
	ReadInputO5m(12, "../iridescent-map", 2035, 1374, featureStore);

	class DrawLibCairoPango drawlib(surface);	
	class MapRender mapRender(&drawlib, 2035, 1374, 12, 2035, 1374, 12, "../iridescent-map");
	mapRender.SetCoastMap(coastMap);
	LabelsByImportance organisedLabels;
	
	mapRender.Render(12, featureStore, true, true, organisedLabels);
	organisedLabelsMap[2035][1374] = organisedLabels;

	// ** Render labels **
	RenderLabelList labelList;
	RenderLabelListOffsets labelOffsets;
	for(int y=1373; y<= 1375; y++)
	{
		for(int x=2034; x <= 2036; x++)
		{
			map<int, LabelsByImportance> &col = organisedLabelsMap[x];
			labelList.push_back(col[y]);
			labelOffsets.push_back(std::pair<double, double>(640.0*(x-2035), 640.0*(y-1374)));
		}
	}

	mapRender.RenderLabels(labelList, labelOffsets);

	cairo_surface_write_to_png(surface, "image.png");	
	cairo_surface_destroy(surface);
	return 0;

}

