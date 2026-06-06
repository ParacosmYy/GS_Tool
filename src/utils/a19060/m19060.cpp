#include "a19060/m19060.h"
QVector<double> m19060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
