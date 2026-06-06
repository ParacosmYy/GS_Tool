#include "k30250/m30250.h"
QVector<double> m30250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
