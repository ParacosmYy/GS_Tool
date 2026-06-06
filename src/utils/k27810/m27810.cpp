#include "k27810/m27810.h"
QVector<double> m27810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
