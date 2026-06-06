#include "a19920/m19920.h"
QVector<double> m19920::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
