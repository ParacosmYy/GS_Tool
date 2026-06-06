#include "n9233/m9233.h"
QVector<double> m9233::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
