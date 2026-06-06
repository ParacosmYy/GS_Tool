#include "i24908/m24908.h"
QVector<double> m24908::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
