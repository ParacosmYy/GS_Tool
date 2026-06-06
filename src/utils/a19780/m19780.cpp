#include "a19780/m19780.h"
QVector<double> m19780::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
