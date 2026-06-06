#include "n9853/m9853.h"
QVector<double> m9853::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
