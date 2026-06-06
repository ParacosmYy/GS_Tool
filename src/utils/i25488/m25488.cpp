#include "i25488/m25488.h"
QVector<double> m25488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
