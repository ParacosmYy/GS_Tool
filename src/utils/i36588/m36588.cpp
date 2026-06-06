#include "i36588/m36588.h"
QVector<double> m36588::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
