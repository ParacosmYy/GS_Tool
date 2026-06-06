#include "i20588/m20588.h"
QVector<double> m20588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
