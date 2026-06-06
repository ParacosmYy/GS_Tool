#include "p21595/m21595.h"
QVector<double> m21595::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
