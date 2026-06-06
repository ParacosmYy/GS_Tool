#include "k19870/m19870.h"
QVector<double> m19870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
