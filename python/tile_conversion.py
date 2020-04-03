#! /usr/bin/env python3

import json
import mercantile
import argparse
import itertools
import pandas as pd
import numpy as np
from geojson import Polygon

version='0.0.0'
print('[tile_remap] Tile Conversion Remapper v {}'.format(version))

# parse cmdline
parser = argparse.ArgumentParser()
parser.add_argument('-c', '--cfg', help='config file', required=True)
args = parser.parse_args()
base = args.cfg[:args.cfg.rfind('.')]

# load config
with open(args.cfg) as f:
  config = json.load(f)

try:
  zoom = config['zoom_lvl']
  tile_in = config['tile_input']
except Exception as e:
  print('EXC in cfg : {}'.format(e))
  exit(1)

# read input tiles
tile_df = pd.read_csv(tile_in, sep=';')
tile_df = tile_df.sort_values(['tileX', 'tileY'])
tile_x = sorted(set(tile_df.tileX.values))
shift_x = { tx : i for i, tx in enumerate(tile_x) }
tile_y = sorted(set(tile_df.tileY.values), reverse=True) # y index increases going south
shift_y = { ty : i for i, ty in enumerate(tile_y) }
tile_list = list(set([ (tx, ty) for tx, ty in tile_df[['tileX', 'tileY']].values ]))

print('Tile in  {}'.format(len(tile_list)))
print('Tile x   {} : {} real {} tot {}'.format(tile_x[0], tile_x[-1], tile_x[-1] - tile_x[0] + 1, len(tile_x)))
print('Tile y   {} : {} real {} tot {}'.format(tile_y[0], tile_y[-1], - tile_y[-1] + tile_y[0] + 1, len(tile_y)))

# # stats
# stats = {
#   'dx' : [],
#   'dy' : []
# }
# for tx, ty in zip(tile_x, tile_y):
#   t = mercantile.Tile(tx, ty, zoom)
#   tile = mercantile.xy_bounds(t)
#   dx = tile.right - tile.left
#   dy = tile.top - tile.bottom
#   stats['dx'].append(dx)
#   stats['dy'].append(dy)
# dxm = np.mean(stats['dx'])
# dym = np.mean(stats['dy'])
# print('dx) mean {} stddev {}'.format(dxm, np.std(stats['dx'])))
# print('dy) mean {} stddev {}'.format(dym, np.std(stats['dy'])))

# mercator box
tile_min = mercantile.Tile(tile_x[0], tile_y[0], zoom)
tile_max = mercantile.Tile(tile_x[-1], tile_y[-1], zoom)
box_min = mercantile.xy_bounds(tile_min)
box_max = mercantile.xy_bounds(tile_max)
xmerc_min = box_min.left
ymerc_min = box_min.bottom
xmerc_max = box_max.right
ymerc_max = box_max.top
dxc = (xmerc_max - xmerc_min) / len(tile_x)
dyc = (ymerc_max - ymerc_min) / len(tile_y)
print('Mercator x) {} {} {}'.format(xmerc_min, xmerc_max, dxc))
print('Mercator y) {} {} {}'.format(ymerc_min, ymerc_max, dyc))

# input tile stretching
data = []
polygons = []
for tx, ty in tile_list:
  i = shift_x[tx]
  j = shift_y[ty]
  bl = mercantile.lnglat( xmerc_min + i * dxc, ymerc_min + j * dyc ) # top left
  tr = mercantile.lnglat( xmerc_min + (i + 1) * dxc, ymerc_min + (j + 1) * dyc ) # bottom right
  lat_min = bl.lat
  lat_max = tr.lat
  lon_min = bl.lng
  lon_max = tr.lng
  data.append([
    tx,
    ty,
    lat_min,
    lat_max,
    lon_min,
    lon_max
  ])
  poly = Polygon([[
    (lon_min, lat_min),
    (lon_max, lat_min),
    (lon_max, lat_max),
    (lon_min, lat_max),
    (lon_min, lat_min)
  ]])
  polygons.append([tx, ty, poly])

# dump geojson
print('Polygons {} {}'.format(len(polygons), len(tile_list)))
geoj = {
  'type' : 'FeatureCollection',
  'features' : [{
    'type': 'Feature',
    'properties': {
      'tileX' : int(tx),
      'tileY' : int(ty)
    },
    'geometry' : poly
  } for tx, ty, poly in polygons ]
}
with open(base + '_maptile.geojson', 'w') as out:
  json.dump(geoj, out, indent=2)

# dump csv
tdf = pd.DataFrame(data, columns=['tileX', 'tileY', 'latMin', 'latMax', 'lonMin', 'lonMax'])
tdf.to_csv(base + '_maptile.csv', sep=';', header=True, index=False)
