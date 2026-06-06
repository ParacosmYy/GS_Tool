#include "k30810/m30810.h"
QVector<double> m30810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
