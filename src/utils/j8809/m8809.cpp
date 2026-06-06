#include "j8809/m8809.h"
QVector<double> m8809::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
