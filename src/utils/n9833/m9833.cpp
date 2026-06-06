#include "n9833/m9833.h"
QVector<double> m9833::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
