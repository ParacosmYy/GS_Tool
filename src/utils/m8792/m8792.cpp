#include "m8792/m8792.h"
QVector<double> m8792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
