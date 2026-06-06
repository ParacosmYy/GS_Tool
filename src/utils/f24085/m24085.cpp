#include "f24085/m24085.h"
QVector<double> m24085::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
