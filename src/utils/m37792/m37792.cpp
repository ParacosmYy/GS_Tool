#include "m37792/m37792.h"
QVector<double> m37792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
