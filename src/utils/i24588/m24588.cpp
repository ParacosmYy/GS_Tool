#include "i24588/m24588.h"
QVector<double> m24588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
