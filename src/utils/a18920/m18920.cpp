#include "a18920/m18920.h"
QVector<double> m18920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
