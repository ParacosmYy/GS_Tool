#include "a9540/m9540.h"
QVector<double> m9540::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
