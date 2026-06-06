#include "k27070/m27070.h"
QVector<double> m27070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
