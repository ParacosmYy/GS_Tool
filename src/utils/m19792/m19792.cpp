#include "m19792/m19792.h"
QVector<double> m19792::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
