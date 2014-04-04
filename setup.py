from distutils.core import setup, Extension

module1 = Extension('bulkredis', sources=['bulkredis.c'])

setup(name='BulkRedis',
      version='1.0',
      description='Efficiently bulk load special format dict into Redis',
      ext_modules=[module1])
