#include "m19112/m19112.h"
QVector<double> m19112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
