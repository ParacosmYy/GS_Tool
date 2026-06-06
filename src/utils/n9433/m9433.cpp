#include "n9433/m9433.h"
QVector<double> m9433::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
