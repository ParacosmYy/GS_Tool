#include "f9905/m9905.h"
QVector<double> m9905::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
