#include "m30792/m30792.h"
QVector<double> m30792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
