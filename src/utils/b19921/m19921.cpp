#include "b19921/m19921.h"
QVector<double> m19921::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
