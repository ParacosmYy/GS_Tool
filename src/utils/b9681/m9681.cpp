#include "b9681/m9681.h"
QVector<double> m9681::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
