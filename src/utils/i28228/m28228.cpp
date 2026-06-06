#include "i28228/m28228.h"
QVector<double> m28228::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
