#include "g18846/m18846.h"
QVector<double> m18846::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
