#include "f24005/m24005.h"
QVector<double> m24005::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
