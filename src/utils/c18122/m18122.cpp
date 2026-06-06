#include "c18122/m18122.h"
QVector<double> m18122::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
