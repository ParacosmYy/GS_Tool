#include "k29190/m29190.h"
QVector<double> m29190::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
