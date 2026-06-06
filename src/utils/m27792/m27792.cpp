#include "m27792/m27792.h"
QVector<double> m27792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
