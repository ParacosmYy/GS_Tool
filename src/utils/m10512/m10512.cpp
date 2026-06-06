#include "m10512/m10512.h"
QVector<double> m10512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
