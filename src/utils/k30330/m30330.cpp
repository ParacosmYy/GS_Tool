#include "k30330/m30330.h"
QVector<double> m30330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
