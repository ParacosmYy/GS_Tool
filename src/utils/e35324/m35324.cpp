#include "e35324/m35324.h"
QVector<double> m35324::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
