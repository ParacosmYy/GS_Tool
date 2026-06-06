#include "k25870/m25870.h"
QVector<double> m25870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
