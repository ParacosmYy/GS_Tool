#include "p19255/m19255.h"
QVector<double> m19255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
