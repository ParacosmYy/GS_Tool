#include "f7905/m7905.h"
QVector<double> m7905::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
