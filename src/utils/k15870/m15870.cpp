#include "k15870/m15870.h"
QVector<double> m15870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
