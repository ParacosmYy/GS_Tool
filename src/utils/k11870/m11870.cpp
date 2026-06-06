#include "k11870/m11870.h"
QVector<double> m11870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
