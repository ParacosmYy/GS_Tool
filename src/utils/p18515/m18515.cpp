#include "p18515/m18515.h"
QVector<double> m18515::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
