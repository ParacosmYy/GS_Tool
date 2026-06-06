#include "k29810/m29810.h"
QVector<double> m29810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
