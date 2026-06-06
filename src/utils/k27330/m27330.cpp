#include "k27330/m27330.h"
QVector<double> m27330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
