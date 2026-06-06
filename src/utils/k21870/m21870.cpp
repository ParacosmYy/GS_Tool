#include "k21870/m21870.h"
QVector<double> m21870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
