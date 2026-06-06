#include "j8029/m8029.h"
QVector<double> m8029::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
