#include "m36032/m36032.h"
QVector<double> m36032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
