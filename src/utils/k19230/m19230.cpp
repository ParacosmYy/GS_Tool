#include "k19230/m19230.h"
QVector<double> m19230::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
