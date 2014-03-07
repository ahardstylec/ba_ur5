from django.conf.urls import patterns, include, url

from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns('',
    url(r'^admin/', include(admin.site.urls)),
    url(r'^files/', include('db_file_storage.urls')),
    url(r'^search/', include('haystack.urls')),
    #url(r'^search/', 'search.views.chemicals'),,
    url(r'^', include('sdb.urls')),
    url(r'^json/add/h_phrase/$', 'jsonadd.views.h_phrase_from_json'),
    url(r'^json/add/r_phrase/$', 'jsonadd.views.r_phrase_from_json'),
    url(r'^json/add/p_phrase/$', 'jsonadd.views.p_phrase_from_json'),
    url(r'^json/add/s_phrase/$', 'jsonadd.views.s_phrase_from_json'),
    url(r'^json/add/eu_danger/$', 'jsonadd.views.eu_from_json'),
    url(r'^json/add/ghs_danger/$', 'jsonadd.views.ghs_from_json'),
    url(r'^json/add/hazard_danger/$', 'jsonadd.views.hazard_from_json'),
    url(r'^json/add/companydata/$', 'jsonadd.views.companydata_from_json'),
    url(r'^json/add/chemical/$', 'jsonadd.views.chemical_from_json'),
)
