#include "c16902/m16902.h"
QVector<double> m16902::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
